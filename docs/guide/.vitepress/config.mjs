import { defineConfig } from 'vitepress'

export default defineConfig({
    title: 'LinNote',
    description: 'A fast, keyboard-driven scratchpad for Linux',

    head: [
        ['link', { rel: 'icon', href: '/icon.png' }],
        ['meta', { name: 'theme-color', content: '#2050c0' }],
        ['link', { rel: 'preconnect', href: 'https://fonts.googleapis.com' }],
        ['link', { rel: 'preconnect', href: 'https://fonts.gstatic.com', crossorigin: '' }],
        ['link', { rel: 'stylesheet', href: 'https://fonts.googleapis.com/css2?family=Space+Grotesk:wght@400;500;600;700&family=Inter:wght@400;500;600&family=JetBrains+Mono:wght@400;500;600&display=swap' }],
        ['meta', { name: 'og:type', content: 'website' }],
        ['meta', { name: 'og:title', content: 'LinNote Documentation' }],
        ['meta', { name: 'og:description', content: 'A fast, keyboard-driven scratchpad for Linux with smart calculator, OCR, timers, and note encryption.' }],
        ['meta', { name: 'og:image', content: 'https://linnote.app/logo.png' }],
        ['meta', { name: 'twitter:card', content: 'summary' }],
    ],

    // Base URL for GitHub Pages deployment
    base: '/guide/',

    // The site is a light theme with one dark beat (code blocks), so the
    // guides follow it rather than forcing dark.
    appearance: false,

    themeConfig: {
        logo: '/logo.png',

        nav: [
            { text: 'Home', link: 'https://linnote.app' },
            { text: 'Install', link: 'https://linnote.app/#install' },
            { text: 'GitHub', link: 'https://github.com/sfnemis/linnote' }
        ],

        sidebar: [
            {
                text: 'Getting Started',
                items: [
                    { text: 'Introduction', link: '/' },
                    { text: 'Installation', link: '/getting-started' }
                ]
            },
            {
                text: 'Features',
                items: [
                    { text: 'Note Modes', link: '/modes' },
                    { text: 'Slash Commands', link: '/commands' },
                    { text: 'Calculator', link: '/calculator' },
                    { text: 'Conversions', link: '/conversions' },
                    { text: 'Timer & Pomodoro', link: '/timer' },
                    { text: 'OCR Screen Capture', link: '/ocr' }
                ]
            },
            {
                text: 'Reference',
                items: [
                    { text: 'Keyboard Shortcuts', link: '/shortcuts' },
                    { text: 'Security & Encryption', link: '/security' },
                    { text: 'Settings', link: '/settings' },
                    { text: 'Troubleshooting', link: '/troubleshooting' }
                ]
            }
        ],

        socialLinks: [
            { icon: 'github', link: 'https://github.com/sfnemis/linnote' }
        ],

        footer: {
            message: 'Released under the MIT License.',
            copyright: 'Copyright © 2025 sfnemis'
        },

        search: {
            provider: 'local'
        },

        editLink: {
            pattern: 'https://github.com/sfnemis/linnote/edit/main/docs/guide/:path',
            text: 'Edit this page on GitHub'
        }
    },

    // Clean URLs without .html extension
    cleanUrls: true,

    markdown: {
        // The page is light with one dark beat: the code blocks. Both Shiki
        // slots therefore get a dark theme, otherwise the light theme's dark
        // tokens land on the dark background and the code is unreadable.
        theme: { light: 'github-dark', dark: 'github-dark' }
    },

    // Sitemap for SEO
    sitemap: {
        hostname: 'https://linnote.app',
        transformItems: (items) => {
            return items.map(item => ({
                ...item,
                url: item.url === '' ? 'guide/' : `guide/${item.url}`
            }))
        }
    }
})

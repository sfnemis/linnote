import { defineConfig } from 'vitepress'

export default defineConfig({
    title: 'LinNote',
    description: 'A fast, keyboard-driven scratchpad for Linux',

    head: [
        ['link', { rel: 'icon', href: '/icon.png' }],
        ['meta', { name: 'theme-color', content: '#8b5cf6' }],
        ['meta', { name: 'og:type', content: 'website' }],
        ['meta', { name: 'og:title', content: 'LinNote Documentation' }],
        ['meta', { name: 'og:description', content: 'A fast, keyboard-driven scratchpad for Linux with smart calculator, OCR, timers, and note encryption.' }],
        ['meta', { name: 'og:image', content: 'https://linnote.app/logo.png' }],
        ['meta', { name: 'twitter:card', content: 'summary' }],
    ],

    // Base URL for GitHub Pages deployment
    base: '/guide/',

    // Default to dark theme to match main site
    // 'force-dark' ensures dark mode regardless of user's system preference
    appearance: 'force-dark',

    themeConfig: {
        logo: '/logo.png',

        nav: [
            { text: 'Home', link: 'https://linnote.app' },
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
